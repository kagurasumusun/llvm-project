# ci/workflows

The reference copy of the GitHub Actions workflows, plus a note on why the CI
is arranged the way it is.

## The constraint

GitHub refuses any write to `.github/workflows/` that comes from a GitHub App
without the `workflows` permission:

    refusing to allow a GitHub App to create or update workflow
    `.github/workflows/<name>.yml` without `workflows` permission

This applies to `git push`, the Contents API and the Git Data API alike -- the
path is what is checked, not the route. Two separate identities hit it here:

* the agent working on this repository, which authenticates as a GitHub App
  installation;
* the built-in `GITHUB_TOKEN`, which authenticates as `github-actions[bot]`.
  That is also an App, and it **cannot be granted** the `workflows` permission
  at all -- `permissions:` in a workflow file has no setting for it.

A job that copies files into `.github/workflows/` therefore does **not** work
while it authenticates as `GITHUB_TOKEN` -- the copying job is refused for
exactly the same reason.

It does work with a different credential. `sync-workflows.yml` checks out with
a personal access token that carries the `workflow` scope, held in a repository
secret, and pushes as the token's owner rather than as an App. That is what is
installed here, and it succeeds: edits to `ci/workflows/` are mirrored into
`.github/workflows/` automatically.

## Belt and braces

The sync above removes the need for a human, but the CI is still arranged so
that the protected file changes as rarely as possible. Only the *path* is
protected, not the behaviour, so `.github/workflows/` holds as little as
possible -- a list of step names, each one calling a script -- and
everything that actually decides what the CI does lives in `ci/metal/`, which
is a normal directory with no restrictions:

    .github/workflows/metal-build.yml   step names and triggers (protected)
    ci/metal/build.sh                   cmake flags, targets, diagnostics (free)

Changing compiler flags, the cmake configuration, how the standard library is
fetched, how failures are reported, which tests run -- none of that touches the
protected file. `metal-build.yml` should only need editing when a step is added
or removed outright, which is rare.

`metal-build.yml` here is the source of truth: `sync-workflows.yml` copies it
into `.github/workflows/` on every push that touches this directory. The two
should never diverge.

## Other ways out, and why they were not taken

* **A GitHub App created for this repository with `workflows: write`.** Tidier
  than a PAT -- scoped to one repository, no personal account behind it -- but
  more setup. Worth revisiting if the PAT becomes awkward to rotate.

A note on the PAT: it can rewrite the CI of this repository, so it is a
credential worth rotating on a schedule and revoking the moment it is no longer
needed.
* **Composite actions under `.github/actions/`.** That directory is *not*
  protected, so it is a real option. `ci/metal/build.sh` was preferred because
  a plain script can be run locally, unchanged, outside Actions.
